import os
import pandas as pd
from pathlib import Path

base_path = Path('/mnt/a/Repositories/Hagen/Masterarbeit-Praktische-Informatik/assets/data/')

class DataStruct:
    def __init__(self, vanilla: pd.DataFrame, staging: pd.DataFrame, environment: str = ''):
        self.vanilla = vanilla
        self.staging = staging
        self.environment = environment

    def set_environment(self, environment: str):
        self.environment = environment

def find_file_base(name: str) -> Path:
    data_dir = base_path
    for path in data_dir.glob('**/*.dat'):
        if name in str(path.absolute()):
            return path.parent
    return None

def read_file(name: str) -> DataStruct:
    base_path = find_file_base(name)

    no_staging = pd.read_csv(base_path / 'hdf5_no_staging.dat', delimiter=' ', header=None)
    staging = pd.read_csv(base_path / 'hdf5_staging.dat', delimiter=' ', header=None)
    return DataStruct(no_staging, staging, base_path.name)

def write_file(data: DataStruct, name: str):
    data_dir = base_path / name / data.environment
    data_dir.mkdir(parents=True, exist_ok=True)
    data.vanilla.to_csv(data_dir / 'hdf5_no_staging.dat', sep=' ', index=False, header=False)
    data.staging.to_csv(data_dir / 'hdf5_staging.dat', sep=' ', index=False, header=False)

def concatenate(data: list[DataStruct], number_of_rows: list[int]) -> DataStruct:
    concatenated = DataStruct(pd.DataFrame(), pd.DataFrame())
    for struct, count in zip(data, number_of_rows):
        concatenated.vanilla = pd.concat([concatenated.vanilla, struct.vanilla.iloc[0:count]], axis=0, ignore_index=True)
        concatenated.staging = pd.concat([concatenated.staging, struct.staging.iloc[0:count]], axis=0, ignore_index=True)
        concatenated.set_environment(struct.environment)

    concatenated.vanilla.iloc[:, 0] = range(0, len(concatenated.vanilla))
    concatenated.staging.iloc[:, 0] = range(0, len(concatenated.staging))    
    
    return concatenated

if __name__ == "__main__":
    # Define the directory containing the dat files
    dat_dir = '/mnt/a/Repositories/Hagen/Masterarbeit-Praktische-Informatik/assets/data/'

    # Call the function to process the dat files
    file1 = read_file('BUFFERED_READ-ALWAYS_THE_SAME-SQUARE-OFFSET-SMALL-ENOUGH_FACTOR_8-FIFO')
    file2 = read_file('BUFFERED_READ-ALWAYS_THE_SAME-SQUARE-OFFSET-MEDIUM-ENOUGH_FACTOR_8-FIFO')
    file3 = read_file('BUFFERED_READ-ALWAYS_THE_SAME-SQUARE-OFFSET-LARGE-ENOUGH_FACTOR_8-FIFO')
    concatenated_file = concatenate([file1, file2, file3], [1, 1, 1])
    write_file(concatenated_file, 'CONCATENATED-BUFFERED_READ-SQUARE-OFFSET-SMALL-MEDIUM-LARGE-ENOUGH_FACTOR_8-FIFO')
    
    file1 = read_file('UNBUFFERED_READ-ALWAYS_THE_SAME-SQUARE-OFFSET-SMALL-ENOUGH_FACTOR_8-FIFO')
    file2 = read_file('UNBUFFERED_READ-ALWAYS_THE_SAME-SQUARE-OFFSET-MEDIUM-ENOUGH_FACTOR_8-FIFO')
    file3 = read_file('UNBUFFERED_READ-ALWAYS_THE_SAME-SQUARE-OFFSET-LARGE-ENOUGH_FACTOR_8-FIFO')
    concatenated_file = concatenate([file1, file2, file3], [1, 1, 1])
    write_file(concatenated_file, 'CONCATENATED-UNBUFFERED_READ-SQUARE-OFFSET-SMALL-MEDIUM-LARGE-ENOUGH_FACTOR_8-FIFO')
    
    file1 = read_file('BUFFERED_READ-ALWAYS_THE_SAME-SQUARE-ALIGNED-SMALL-ENOUGH_FACTOR_8-FIFO')
    file2 = read_file('BUFFERED_READ-ALWAYS_THE_SAME-SQUARE-ALIGNED-MEDIUM-ENOUGH_FACTOR_8-FIFO')
    file3 = read_file('BUFFERED_READ-ALWAYS_THE_SAME-SQUARE-ALIGNED-LARGE-ENOUGH_FACTOR_8-FIFO')
    concatenated_file = concatenate([file1, file2, file3], [1, 1, 1])
    write_file(concatenated_file, 'CONCATENATED-BUFFERED_READ-SQUARE-ALIGNED-SMALL-MEDIUM-LARGE-ENOUGH_FACTOR_8-FIFO')
    
    file1 = read_file('UNBUFFERED_READ-ALWAYS_THE_SAME-SQUARE-ALIGNED-SMALL-ENOUGH_FACTOR_8-FIFO')
    file2 = read_file('UNBUFFERED_READ-ALWAYS_THE_SAME-SQUARE-ALIGNED-MEDIUM-ENOUGH_FACTOR_8-FIFO')
    file3 = read_file('UNBUFFERED_READ-ALWAYS_THE_SAME-SQUARE-ALIGNED-LARGE-ENOUGH_FACTOR_8-FIFO')
    concatenated_file = concatenate([file1, file2, file3], [1, 1, 1])
    write_file(concatenated_file, 'CONCATENATED-UNBUFFERED_READ-SQUARE-ALIGNED-SMALL-MEDIUM-LARGE-ENOUGH_FACTOR_8-FIFO')

    pass
