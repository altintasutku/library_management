#ifndef AUTH_H
#define AUTH_H

typedef enum {
    ROLE_ADMIN,
    ROLE_LIBRARIAN,
    ROLE_USER
} UserRole;

typedef struct {
    char username[100];
    UserRole role;
    // Diğer kullanıcı bilgileri...
} User;

User* authenticate_user(const char* username, const char* password);
bool has_permission(const User* user, const char* action);

#endif // AUTH_H 