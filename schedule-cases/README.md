# Schedule Cases

These cases are designed for testing the scheduling policy.

## How to create schedule cases

- Write your code in files located in the partition directory, such as [`pr1/main.c`](spmt/pr1/main.c), [`pr1/activity.c`](spmt/pr1/activity.c) and [`pr1/activity.h`](spmt/pr1/activity.h).
- Create a YAML configuration file in the root directory of the case, for example, [`config.yaml`](spmt/config.yaml).
- Navigate to the directory containing your `config.yaml` file. (Important!)
- Use the following command to generate deployment files:
    ```
    ../../misc/gen_deployment config.yaml
    ```
Now, your schedule case should be ready for building and execution.
