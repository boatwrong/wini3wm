# Program Outline

1. init
    - set hotkeys

2. windows event loop - catch the following events/messages
    - new window opened
        - assign to active workspace
    - hotkey pressed
        - hotkey + shift => move window to workspace, and jump to workspace
        - hotkey => jump to workspace

    (after any event, retile the active workspace)

3. Teardown
