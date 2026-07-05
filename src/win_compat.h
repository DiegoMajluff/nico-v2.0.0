/*
 * Nico v2.0.0 - Intérprete Educativo de Scripting en Español
 * @file:         win_compat.h
 * @author:       Diego Alejandro Majluff (Diseño, Arquitectura y Supervisión)
 * @ai_assist:    Qwen (Alibaba Cloud) - Implementación, Debugging y Optimización
 * @license:      MIT / Personal Use (ver LICENSE)
 * @description:  Capa de compatibilidad cross-platform para Windows (MinGW-w64).
 *                Provee stubs y wrappers de funciones POSIX (sleep, usleep, strcasecmp,
 *                poll, termios) y configuración de sockets (winsock2) para permitir
 *                que el código base escrito para Linux/Unix compile y ejecute correctamente
 *                en entornos Windows sin modificaciones en la lógica principal.
 */
#ifndef WIN_COMPAT_H
#define WIN_COMPAT_H

#ifdef _WIN32
#include <windows.h>
#include <string.h>
#include <io.h>
#include <conio.h> 

// File descriptors estándar
#define STDIN_FILENO  0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

// sleep() → Sleep()
#define sleep(sec) Sleep((DWORD)((sec) * 1000))

// usleep() → Sleep() con conversión µs → ms
static inline int nico_usleep_win(unsigned int usec) {
    if (usec > 0) {
        DWORD ms = (DWORD)(usec / 1000 + (usec % 1000 > 0 ? 1 : 0));
        Sleep(ms);
    }
    return 0;
}
#define usleep(usec) nico_usleep_win(usec)

// Comparación de strings case-insensitive
#define strcasecmp(s1, s2) _stricmp(s1, s2)
#define strncasecmp(s1, s2, n) _strnicmp(s1, s2, n)

// Socket API: winsock2.h explícito para compatibilidad XP
#if defined(__MINGW64__) && defined(_WIN32_WINNT) && _WIN32_WINNT >= 0x0501
    #include <winsock2.h>      // ← Necesario para XP
    #include <ws2tcpip.h>
    #define NicoUseWSAPoll 1
#else
    // Fallback minimalista para poll() en stdin (teclado)
    #define POLLIN 1
    struct pollfd {
        int fd;
        short events;
        short revents;
    };
    static inline int poll(struct pollfd *fds, unsigned int nfds, int timeout) {
        (void)timeout; (void)nfds;
        if (fds[0].fd == STDIN_FILENO) {
            fds[0].revents = _kbhit() ? POLLIN : 0;
            return fds[0].revents ? 1 : 0;
        }
        return 0;
    }
    #define NicoUseWSAPoll 0
#endif

// Stubs de termios: no se usan realmente porque io.c usa WinAPI directo
struct termios { int dummy; };
#define TCSANOW 0
#define TCSADRAIN 0
#define ICANON 0
#define ECHO 0
#define VMIN 0
#define VTIME 0
static inline int tcgetattr(int fd, struct termios *t) { (void)fd; (void)t; return 0; }
static inline int tcsetattr(int fd, int opt, const struct termios *t) { (void)fd; (void)opt; (void)t; return 0; }
static inline int tcdrain(int fd) { (void)fd; return 0; }

#endif // _WIN32
#endif // WIN_COMPAT_H