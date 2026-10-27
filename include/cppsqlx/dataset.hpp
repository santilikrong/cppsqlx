
#ifndef DATASET_HPP
#define DATASET_HPP

#include <cppsqlx/column.hpp>
#include <cppsqlx/joinable.hpp>
#include <map>
#include <string>
#include <optional>

namespace cppsqlx
{

class Join;

class Dataset : public Joinable
{
public:
    Dataset(std::string name);

    std::string xml_;
    std::string name_;
    std::string schema_;
    std::string catalog_;
    std::string alias_;
    std::map<int, Column> columns_;

public:
    Dataset();
    virtual ~Dataset();

    // Accessors
    std::string name() const;
    std::string schema() const;
    std::string catalog() const;
    std::string alias() const;
    std::string toString() const;

    // Mutators
    void setSchema(std::string schema);
    void setCatalog(std::string catalog);
    Dataset& as(std::string alias);

    // Column access
    std::optional<Column> at(int index) const;
    int rowSize() const;

    // Joinable interface
    std::string joinidentifier() const override;

    // Join operation
    Join joinTo(const Dataset& rightds) const;
};

} // namespace cppsqlx

#endif //

