" Custom .vimrc for this repo, please let your vim load this .vimrc properly.

" For Pebble C headers
let g:syntastic_c_include_dirs = ['~/pebble-dev/PebbleSDK-3.7/Pebble/*/include']

" Since C library dirs may not set properly in same cases, so let's turn syntastic off here.
let g:syntastic_always_populate_loc_list = 0
let g:syntastic_auto_loc_list = 0
let g:syntastic_check_on_open = 0
let g:syntastic_check_on_wq = 0
let g:syntastic_enable_signs = 0

" Use 2 spaces for tabs
set shiftwidth=2

" Shortcuts
map <leader>m :w<CR>!make run<CR>
