class Layout
  include Inesita::Layout

  def toggle_settings(evt)
    if evt.target.id == 'settings-overlay'
      store.settings_toggle
    end
  end

  def toggle_album_details(evt)
    if evt.target.id == 'album-details-container'
      $document.at('#album-details-container').hide
      store.album_details_clear
    end
  end

  def render
    div id: 'top-container' do
      component Top
    end

    div id: 'outlet-container' do
      component outlet
    end

    div id: 'settings-overlay', onclick: method(:toggle_settings) do
      component Settings
    end

    div id: 'notification-overlay' do
      component Notification
    end

    div id: 'album-details-container', onclick: method(:toggle_album_details) do
      div id: 'album-details' do
        component AlbumDetails
      end
    end

    div id: 'connection-lost-overlay' do
      component ConnectionLost
    end
  end
end
