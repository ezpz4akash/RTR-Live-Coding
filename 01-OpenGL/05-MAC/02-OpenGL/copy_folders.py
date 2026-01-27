import os
import shutil

def copy_folder_structure(src, dst):
    """
    Recursively copies all folders from src to dst without copying any files.
    """
    src = os.path.abspath(src)
    dst = os.path.abspath(dst)

    if not os.path.exists(src):
        print(f"Source path does not exist: {src}")
        return

    for root, dirs, files in os.walk(src):
        # Compute the destination path for this root
        rel_path = os.path.relpath(root, src)
        dest_dir = os.path.join(dst, rel_path)

        # Create destination directory if it doesn't exist
        os.makedirs(dest_dir, exist_ok=True)

        # Skip copying files — only create directories
        for d in dirs:
            new_dir_path = os.path.join(dest_dir, d)
            os.makedirs(new_dir_path, exist_ok=True)

    print(f"Folder structure copied successfully from:\n{src}\n→ {dst}")


if __name__ == "__main__":
    # Example usage:
    # Replace these with your source and destination paths
    src_folder = r"D:\\Projects\\rtr06_158_vagish_adhav\\01-OpenGL\\01-Windows\\03-PP"
    dst_folder = r"."

    copy_folder_structure(src_folder, dst_folder)
