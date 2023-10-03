@Colors = class Colors
  constructor: ->
    @entries = [
      { color: 'rgb(0, 173, 0)' },
      { color: 'rgb(255, 40, 255)' },
      { color: 'rgb(49, 151, 255)' },
      { color: 'rgb(187, 142, 29)' },
      { color: 'rgb(255, 93, 94)' },
      { color: 'rgb(28, 166, 159)' },
      { color: 'rgb(199, 117, 219)' },
      { color: 'rgb(255, 97, 38)' },
    ]
  
  hideHighlights: ->
    @entries.forEach (entry) ->
      entry.div?.remove()
      entry.threadNumber = null
  
  destroyHighlights: ->
    @entries.forEach (entry) ->
      entry.div?.remove()
      entry.div = null
      entry.threadNumber = null

  getHighlightDiv: (threadNumber) ->
    entry = @entries.find (el) -> not el.threadNumber?
    if entry?
      entry.threadNumber = threadNumber
      unless entry.div?
        entry.div = document.createElement 'div'
        entry.div.classList.add 'highlight'
        entry.div.style = "--highlight-color: #{entry.color};"
      entry.div

  allColors: ->
    @entries.map (el) -> el.color
  
  addColor: (color) ->
    @dummy ?= document.createElement 'div'
    @dummy.style.backgroundColor = color
    @entries.push color: @dummy.style.backgroundColor
  
  removeColor: (color) ->
    if typeof color is 'string'
      idx = @entries.findIndex (el) -> el.color is color
      return false if idx < 0
    else if color of @entries
      idx = color
    else
      return false
    entry = @entries[idx]
    if entry.threadNumber?
      replacement = @entries.find (el) -> not el.threadNumber?
      return false unless replacement?
      entry.div?.style.setProperty '--highlight-color', replacement.color
      replacement.div = entry.div
      replacement.threadNumber = entry.threadNumber
    @entries.splice idx, 1
    true

  replaceColor: (before, after) ->
    if typeof before is 'string'
      entry = @entries.find (el) -> el.color is before
      return false unless entry?
    else if before of @entries
      entry = @entries[before]
    else
      return false

    @dummy ?= document.createElement 'div'
    @dummy.style.backgroundColor = after
    entry.color = @dummy.style.backgroundColor
    entry.div?.style.setProperty '--highlight-color', entry.color
    true

@Colors::[Symbol.toStringTag] = 'Colors'
