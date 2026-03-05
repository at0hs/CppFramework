#ifndef INCLUDE_SUBPROCESS_PROCESSSPAWNER_HPP
#define INCLUDE_SUBPROCESS_PROCESSSPAWNER_HPP

#include <unistd.h>

namespace Framework::SubProcess {

	class ProcessSpawner {
	public:
		ProcessSpawner() = delete;
		~ProcessSpawner() = delete;

		ProcessSpawner(const ProcessSpawner &) = delete;
		ProcessSpawner &operator=(const ProcessSpawner &) = delete;
		ProcessSpawner(ProcessSpawner &&) = delete;
		ProcessSpawner &operator=(ProcessSpawner &&) = delete;

		template <typename F, typename T>
		static pid_t spawn(F function, T *argument) {
			pid_t pid = fork();
			if (pid == 0) {
				int ret = function(argument);
				_exit(ret);
			}
			return pid;
		}
	};
} // namespace Framework::SubProcess

#endif // INCLUDE_SUBPROCESS_PROCESSSPAWNER_HPP
