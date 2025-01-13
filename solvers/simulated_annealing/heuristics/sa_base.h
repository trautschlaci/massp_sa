#ifndef SA_BASE_H
#define SA_BASE_H


class SABase {
public:
    virtual ~SABase() = default;

    virtual bool is_finished() const = 0;
    virtual bool do_accept(const long long int& cost_delta) = 0;
    virtual void tick(bool was_accepted) = 0;
};

#endif //SA_BASE_H
