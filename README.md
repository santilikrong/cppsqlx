# cppsqlx

**cppsqlx** is a modern **C++23** PostgreSQL SQL builder, metadata discovery engine, and parallel data-integration workflow library built on top of `libpq` and `nlohmann/json`.

---

## ✨ Key Features

### 🧱 Type-Safe Fluent SQL Query Builder
* **Full DML & DDL Coverage:** Expressive builders for `SELECT`, `INSERT`, `UPDATE`, `DELETE`, `CREATE TABLE AS SELECT (CTAS)`, and `DROP TABLE`.
* **Advanced SQL Constructs:** Native support for Common Table Expressions (`WITH` / CTEs), `INNER`/`LEFT`/`RIGHT`/`FULL` joins, set operations (`UNION`, `UNION ALL`, `INTERSECT`, `EXCEPT`), `GROUP BY`, `HAVING`, `ORDER BY`, `LIMIT`/`OFFSET`, and raw SQL / window expressions (`OVER (...)`).
* **Compile-Time & Runtime Validation:** Built-in query validation (`validate()`) verifying branch column compatibility and structural integrity before execution.

### 🔍 Database Metadata & Reverse Engineering
* **Catalog Introspection:** Discover schemas, tables, and detailed column metadata (data types, nullability, primary keys) directly from PostgreSQL catalogs via `DatabaseMetadata` and `DatasetFactory`.

### 🔌 Environment & Connection Abstraction
* **Physical & Logical Connections:** Separate physical cluster credentials (`PhysicalConnection`) from environment-aware logical gateways (`LogicalConnection` mapping `DEV`, `TEST`, and `PROD` contexts).

### ⚙️ Parallel Workflow & ETL Execution Engine
* **Multi-Step Tasks & Jobs:** Compose sequential SQL steps (`UnitSelect` / `QueryVariant`) into reusable `Task` units and dependency-aware `Job` nodes.
* **Parallel DAG Execution:** `Workflow::run()` automatically resolves job dependencies, spawns dedicated worker threads (`std::jthread`) with isolated database connections, and reports real-time step-by-step progress callbacks.

### 💾 Full JSON Serialization
* **Project Persistence:** Serialize and deserialize entire `DataIntegrationProject` models—including connections, datasets, queries, logical query folders, tasks, jobs, and workflows—to and from JSON (`toJSON()` / `fromJSON()`).

---

## 🛠️ Requirements

* **Compiler:** C++23 compatible compiler (`GCC 13+`, `Clang 16+`, or `MSVC 19.36+`)
* **Build System:** `CMake 3.20+` and `Ninja` (or `Make`)
* **Dependencies:**
  * PostgreSQL client library (`libpq`)
  * `nlohmann/json`
  * `log4cxx` (logging subsystem)

---

## 🚀 Building & Installing

```bash
# Configure the library
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release

# Compile
ninja -C build

# Install headers and library target (for downstream projects like dizui Studio)
sudo cmake --install build
```

---

## 💡 Quick Usage Example

```cpp
#include <cppsqlx/connection.hpp>
#include <cppsqlx/selectquery.hpp>
#include <cppsqlx/logicalconnection.hpp>
#include <cppsqlx/workflow.hpp>
#include <iostream>

using namespace cppsqlx::literals;

int main()
{
    // 1. Build a type-safe SELECT query
    cppsqlx::SelectQuery query;
    query.select("p.product_name"_col.as("product"), "g.group_name"_col.as("category"), "p.price"_col)
         .from(std::make_shared<cppsqlx::Dataset>("public.products"), "p")
         .join("INNER JOIN", std::make_shared<cppsqlx::Dataset>("public.product_groups"), "g",
               "p.group_id"_col.equal("g.group_id"_col))
         .where("p.price"_col.greater("100"_col))
         .orderBy("p.price"_col);

    std::cout << query.toSQL() << std::endl;

    // 2. Configure an environment-aware LogicalConnection
    cppsqlx::PhysicalConnection devDb;
    devDb.name_ = "local_pg";
    devDb.dbhost_ = "localhost";
    devDb.dbport_ = 5432;
    devDb.dbname_ = "analytics";
    devDb.dbuser_ = "postgres";

    cppsqlx::LogicalConnection gateway("DWH_Gateway");
    gateway.registerEnvironment("DEV", devDb);

    return 0;
}
```

---

## 📁 Library Architecture

```text
cppsqlx/
├── include/cppsqlx/
│   ├── connection.hpp                 # libpq session & statement wrapper
│   ├── physicalconnection.hpp         # Cluster connection parameters
│   ├── logicalconnection.hpp          # DEV/TEST/PROD environment router
│   ├── sqlexpression.hpp              # Fluent SQL expression & operator AST
│   ├── selectquery.hpp                # SELECT, JOIN, CTE & UNION query builder
│   ├── dataset.hpp                    # Table & schema representation
│   ├── task.hpp / job.hpp             # Sequential SQL execution units
│   ├── workflow.hpp                   # Parallel DAG orchestrator (std::jthread)
│   └── data_integration_project.hpp   # Root project container & JSON persistence
├── src/                               # Implementation files
└── CMakeLists.txt                     # Library build & CMake package export
```

---

## 📄 License

This project is licensed under the MIT License. See the `LICENSE` file for details.
