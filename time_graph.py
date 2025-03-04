import matplotlib.pyplot as plt
import re

def parse_file(filename):
    mark_durations = []
    sweep_durations = []
    mark_objects_collected = {}
    
    mark_start_times = {}
    sweep_start_times = {}
    
    with open(filename, 'r') as file:
        for line in file:
            match = re.match(r'\[(\d+\.\d+)\] (\w+) stage (\d+) (started|completed).', line)
            if match:
                time = float(match.group(1))
                stage_type = match.group(2)
                stage_number = int(match.group(3))
                status = match.group(4)
                
                if stage_type == 'Mark':
                    if status == 'started':
                        mark_start_times[stage_number] = time
                    elif status == 'completed':
                        if stage_number in mark_start_times:
                            duration = time - mark_start_times[stage_number]
                            mark_durations.append((stage_number, duration))
                
                elif stage_type == 'Sweep':
                    if status == 'started':
                        sweep_start_times[stage_number] = time
                    elif status == 'completed':
                        if stage_number in sweep_start_times:
                            duration = time - sweep_start_times[stage_number]
                            sweep_durations.append((stage_number, duration))
            
            match_objects = re.match(r'\[(\d+\.\d+)\] Mark stage (\d+) completed. Objects collected: (\d+).', line)
            if match_objects:
                stage_number = int(match_objects.group(2))
                objects_collected = int(match_objects.group(3))
                mark_objects_collected[stage_number] = objects_collected
    
    return mark_durations, sweep_durations, mark_objects_collected

def plot_durations_and_objects(mark_durations, sweep_durations, mark_objects_collected):
    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(8, 10))
    
    mark_stages, mark_durations = zip(*mark_durations)
    sweep_stages, sweep_durations = zip(*sweep_durations)
    
    mark_durations = [int(duration * 1000000) for duration in mark_durations]
    sweep_durations = [int(duration * 1000000) for duration in sweep_durations]
    
    ax1.plot(mark_stages, mark_durations, label='Mark stage duration', marker='o')
    ax1.plot(sweep_stages, sweep_durations, label='Sweep stage duration', marker='o')
    
    ax1.set_xticks(mark_stages)
    ax1.set_yticks(list(set(mark_durations + sweep_durations)))
    ax1.set_xlabel('Stage Number')
    ax1.set_ylabel('Duration (microseconds)')
    ax1.set_title('Mark and Sweep Stage Durations')
    ax1.legend()
    ax1.grid(True)
    
    stages = list(mark_objects_collected.keys())
    objects_collected = list(mark_objects_collected.values())
    
    ax2.bar(stages, objects_collected, label='Objects Collected', color='orange')
    
    ax2.set_xticks(stages)
    ax2.set_yticks(objects_collected)
    ax2.set_xlabel('Mark Stage Number')
    ax2.set_ylabel('Objects Collected')
    ax2.set_title('Objects Collected per Mark Stage')
    ax2.legend()
    ax2.grid(True)
    
    plt.subplots_adjust(hspace=0.3)
    
    plt.show()

if __name__ == "__main__":
    filename = input("Enter the filename: ")
    mark_durations, sweep_durations, mark_objects_collected = parse_file(filename)
    plot_durations_and_objects(mark_durations, sweep_durations, mark_objects_collected)