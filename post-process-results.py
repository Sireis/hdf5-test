import os
import pandas as pd
from pathlib import Path

base_path = Path('/mnt/a/Repositories/Hagen/Masterarbeit-Praktische-Informatik/assets/data/')

class DataStruct:
    def __init__(self, vanilla: pd.DataFrame, staging: pd.DataFrame, environment: str):
        self.vanilla = vanilla
        self.staging = staging
        self.environment = environment

class Result:
    def __init__(self, data: list[DataStruct]):
        self.data = data

    def set_enivronment(self, environment: str):
        self.environment = environment

def find_file_base(name: str) -> Path:
    data_dir = base_path
    for path in data_dir.glob('**/*.dat'):
        if name in str(path.absolute()):
            return path.parent.parent
    return None

def read_result(name: str) -> DataStruct:
    base_path = find_file_base(name)

    result = Result([])

    for folder in base_path.iterdir():
        if folder.is_dir():
            environment = folder.name
            no_staging = pd.read_csv(folder / 'hdf5_no_staging.dat', delimiter=' ', header=None)
            staging = pd.read_csv(folder / 'hdf5_staging.dat', delimiter=' ', header=None)

            result.data.append(DataStruct(no_staging, staging, environment))

    return result

def write_result(result: Result, name: str):
    for data in result.data:
        path = base_path / name / data.environment
        path.mkdir(parents=True, exist_ok=True)
        data.vanilla.to_csv(path / 'hdf5_no_staging.dat', sep=' ', index=False, header=False)
        data.staging.to_csv(path / 'hdf5_staging.dat', sep=' ', index=False, header=False)

def concatenate(results: list[Result], number_of_rows: list[int]) -> Result:
    concatenated = Result([])
    
    for original in results[0].data:
        concatenated.data.append(DataStruct(pd.DataFrame(), pd.DataFrame(), original.environment))

    for result, count in zip(results, number_of_rows):
        for original, stacked in zip(result.data, concatenated.data):
            stacked.vanilla = pd.concat([stacked.vanilla, original.vanilla.iloc[0:count]], axis=0, ignore_index=True)
            stacked.staging = pd.concat([stacked.staging, original.staging.iloc[0:count]], axis=0, ignore_index=True)

    for stacked in concatenated.data:
        stacked.vanilla.iloc[:, 0] = range(0, len(stacked.vanilla))
        stacked.staging.iloc[:, 0] = range(0, len(stacked.staging))    
    
    return concatenated

if __name__ == "__main__":
    # Define the directory containing the dat files
    dat_dir = '/mnt/a/Repositories/Hagen/Masterarbeit-Praktische-Informatik/assets/data/'

    # Call the function to process the dat files
    file1 = read_result('BUFFERED_READ-ALWAYS_THE_SAME-SQUARE-OFFSET-SMALL-ENOUGH_FACTOR_8-FIFO')
    file2 = read_result('BUFFERED_READ-ALWAYS_THE_SAME-SQUARE-OFFSET-MEDIUM-ENOUGH_FACTOR_8-FIFO')
    file3 = read_result('BUFFERED_READ-ALWAYS_THE_SAME-SQUARE-OFFSET-LARGE-ENOUGH_FACTOR_8-FIFO')
    concatenated_file = concatenate([file1, file2, file3], [1, 1, 1])
    write_result(concatenated_file, 'CONCATENATED-BUFFERED_READ-SQUARE-OFFSET-SMALL-MEDIUM-LARGE-ENOUGH_FACTOR_8-FIFO')
    
    file1 = read_result('UNBUFFERED_READ-ALWAYS_THE_SAME-SQUARE-OFFSET-SMALL-ENOUGH_FACTOR_8-FIFO')
    file2 = read_result('UNBUFFERED_READ-ALWAYS_THE_SAME-SQUARE-OFFSET-MEDIUM-ENOUGH_FACTOR_8-FIFO')
    file3 = read_result('UNBUFFERED_READ-ALWAYS_THE_SAME-SQUARE-OFFSET-LARGE-ENOUGH_FACTOR_8-FIFO')
    concatenated_file = concatenate([file1, file2, file3], [1, 1, 1])
    write_result(concatenated_file, 'CONCATENATED-UNBUFFERED_READ-SQUARE-OFFSET-SMALL-MEDIUM-LARGE-ENOUGH_FACTOR_8-FIFO')
    
    file1 = read_result('BUFFERED_READ-ALWAYS_THE_SAME-SQUARE-ALIGNED-SMALL-ENOUGH_FACTOR_8-FIFO')
    file2 = read_result('BUFFERED_READ-ALWAYS_THE_SAME-SQUARE-ALIGNED-MEDIUM-ENOUGH_FACTOR_8-FIFO')
    file3 = read_result('BUFFERED_READ-ALWAYS_THE_SAME-SQUARE-ALIGNED-LARGE-ENOUGH_FACTOR_8-FIFO')
    concatenated_file = concatenate([file1, file2, file3], [1, 1, 1])
    write_result(concatenated_file, 'CONCATENATED-BUFFERED_READ-SQUARE-ALIGNED-SMALL-MEDIUM-LARGE-ENOUGH_FACTOR_8-FIFO')
    
    file1 = read_result('UNBUFFERED_READ-ALWAYS_THE_SAME-SQUARE-ALIGNED-SMALL-ENOUGH_FACTOR_8-FIFO')
    file2 = read_result('UNBUFFERED_READ-ALWAYS_THE_SAME-SQUARE-ALIGNED-MEDIUM-ENOUGH_FACTOR_8-FIFO')
    file3 = read_result('UNBUFFERED_READ-ALWAYS_THE_SAME-SQUARE-ALIGNED-LARGE-ENOUGH_FACTOR_8-FIFO')
    concatenated_file = concatenate([file1, file2, file3], [1, 1, 1])
    write_result(concatenated_file, 'CONCATENATED-UNBUFFERED_READ-SQUARE-ALIGNED-SMALL-MEDIUM-LARGE-ENOUGH_FACTOR_8-FIFO')

    pass
