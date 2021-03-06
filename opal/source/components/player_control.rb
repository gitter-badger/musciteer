class PlayerControl
  include Inesita::Component

  def init
    store.on(:player_state_changed) do
      $document.at('#player-ctrl').show if store.player_state == :stopped
    end
  end

  def render
    div do
      div do
        # spacing
      end
      div do
        #spacing
      end
      div id: 'player-toggle' do
        case store.player_state
        when :stopped
          button onclick: -> { store.player_play } do
            img src: 'assets/play.svg'
          end
        when :playing
          button onclick: -> { store.player_stop } do
            img src: 'assets/stop.svg'
          end
        end
      end
      div id: 'player-skip' do
        case store.player_state
        when :stopped
        when :playing
          button onclick: -> { store.player_skip } do
            img src: 'assets/skip.svg'
          end
        end
      end
      div do
        button onclick: -> { `document.documentElement.webkitRequestFullscreen()` } do
          text "fullscreen"
        end
      end
    end
  end
end
