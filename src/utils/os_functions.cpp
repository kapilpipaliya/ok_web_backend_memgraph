#include "utils/os_functions.hpp"
#include <drogon/drogon.h>
#include "utils/GlobalSettings.hpp"
#include <unistd.h>
#include <poll.h>
#include <cstdlib>
#include <trantor/utils/Logger.h>
#include <sys/inotify.h>
#include <functional>
#include <filesystem>
namespace ok
{
namespace utils::file
{
void assertDirectoryExist(const std::vector<std::string> &paths){
  for(const auto &path: paths){
    if (!(std::filesystem::exists(path) && std::filesystem::is_directory(path)))
    {
      LOG_DEBUG << "path directory not exist: " << path;
      LOG_DEBUG << "current path is: " << std::filesystem::current_path().string();
      exit(1);
    }
  }
}
void watchFile(std::string const &pathname, std::function<void()> func) noexcept
{
  int poll_num;
  auto fileDescriptor = inotify_init1(IN_NONBLOCK);
  if (fileDescriptor == -1)
  {
    perror("inotify_init1");
    switch (fileDescriptor)
    {
      case EMFILE:
        LOG_DEBUG << "The user limit on the total number of inotify "
                     "instances "
                     "has been reached.";
        break;
      case ENFILE:
        LOG_DEBUG << "The system limit on the total number of file "
                     "descriptors "
                     "has been reached.";
        break;
      case ENOMEM: LOG_DEBUG << "Insufficient kernel memory is available."; break;
    }
  }
  auto uniqueWatchDescriptor = inotify_add_watch(fileDescriptor, pathname.c_str(), IN_CLOSE_WRITE);
  if (uniqueWatchDescriptor == -1)
  {
    fprintf(stderr, "Cannot watch '%s'\n", pathname.c_str());
    perror("inotify_add_watch");
    switch (uniqueWatchDescriptor)
    {
      case EBADF: LOG_DEBUG << "The given file descriptor is not valid."; break;
      case EFAULT:
        LOG_DEBUG << "pathname points outside of the process’s accessible "
                     "address space.";
        break;
      case EINVAL:
        LOG_DEBUG << "The given event mask contains no legal events; "
                     "or fd is "
                     "not an inotify file descriptor.";
        break;
      case ENOMEM: LOG_DEBUG << "Insufficient kernel memory was available."; break;
      case ENOSPC:
        LOG_DEBUG << "	The user limit on the total number of inotify "
                     "watches "
                     "was reached or the kernel failed to allocate a needed "
                     "resource.";
        break;
      default: break;
    }
  }
  /* Prepare for polling */
  nfds_t nfds;
  struct pollfd fds[1];
  nfds = 1;
  /* Inotify input */
  fds[0].fd = fileDescriptor;
  fds[0].events = POLLIN;
  /* Wait for events and/or terminal input */
  printf("Listening for events.\n");
  while (true)
  {
    poll_num = poll(fds, nfds, -1);
    if (poll_num == -1)
    {
      if (errno == EINTR) continue;
      perror("poll");
      exit(EXIT_FAILURE);
    }
    if (poll_num > 0)
    {
      if (fds[0].revents & POLLIN)
      {
        /* Inotify events are available */
        impl::handleEvents(fileDescriptor, uniqueWatchDescriptor, func);
      }
    }
  }
  printf("Listening for events stopped.\n");
  /* Close inotify file descriptor */
  close(fileDescriptor);
}
namespace impl
{
// Read all available inotify events from the file descriptor 'fd'.
// wd is the table of watch descriptors for the directories in argv.
// argc is the length of wd and argv.
// argv is the list of watched directories.
// Entry 0 of wd and argv is unused.
void handleEvents(int fileDescriptor, int uniqueWatchDescriptor, std::function<void()> func) noexcept
{
  /* Some systems cannot read integer variables if they are not
          properly aligned. On other systems, incorrect alignment may
          decrease performance. Hence, the buffer used for reading from
          the inotify file descriptor should have the same alignment as
          struct inotify_event. */
  char buf[4096] __attribute__((aligned(__alignof__(struct inotify_event))));
  const struct inotify_event *event;
  ssize_t len;
  char *ptr;
  /* Loop while events can be read from inotify file descriptor. */
  for (;;)
  {
    /* Read some events. */
    len = read(fileDescriptor, buf, sizeof buf);
    if (len == -1 && errno != EAGAIN)
    {
      perror("read");
      exit(EXIT_FAILURE);
    }
    /* If the nonblocking read() found no events to read, then
          it returns -1 with errno set to EAGAIN. In that case,
          we exit the loop. */
    if (len <= 0) break;
    /* Loop over all events in the buffer */
    for (ptr = buf; ptr < buf + len; ptr += sizeof(struct inotify_event) + event->len)
    {
      event = (const struct inotify_event *)ptr;
      /* Print event type */
      if (event->mask & IN_CLOSE_WRITE)
      {
        func();
        /* Print the name of the watched directory */
        //            for (i = 1; i < argc; ++i) {
        //                if (wd[i] == event->wd) {
        //                    printf("%s/", argv[i]);
        //                    break;
        //                }
        //            }
      }
      /* Print the name of the file */
      if (event->len) { LOG_DEBUG << event->name; }
      /* Print type of filesystem object */
      if (event->mask & IN_ISDIR) { LOG_DEBUG << " [directory]\n"; }
      else
      {
        LOG_DEBUG << " [file]\n";
      }
    }
  }
}
}  // namespace impl
}  // namespace utils::file
}  // namespace ok
