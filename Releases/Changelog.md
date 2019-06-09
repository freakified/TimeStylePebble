# TimeStyle Versions

## Release-7.1
_Published on 2017-01-11 15:48:12 UTC_

Hardware Platforms:
Aplite, Basalt, Chalk, Diorite, Emery

Release Notes:
v7.1
* Oops left in a debug line that broke the config page. Fixed now.

## Release-7.0
_Published on 2017-01-11 15:06:17 UTC_

Hardware Platforms:
Aplite, Basalt, Chalk, Diorite, Emery

Release Notes:
Version 7.0 ("The Apocalypse Update")
	• Support for Time 2, so that when you pay $2000 for a prototype Time 2 on Ebay you'll be able to run TimeStyle on it
	• Added option to hide the Bluetooth disconnection icon, for when the Pebble app is but a distant memory
		○ (Thanks to plarus on GitHub for this contribution!)
	• Fixed visual glitch when LECO is selected and leading zero is hidden
	• Added the Hebrew language (language pack required) so that you'll be able to converse with the four horsemen of the apocalypse

## Release-6.11
_Published on 2016-11-25 13:33:42 UTC_

Hardware Platforms:
Aplite, Basalt, Chalk, Diorite

Release Notes:
v6.11
 - Vibration settings now respect watch quiet time setting

## Release-6.10
_Published on 2016-11-03 14:07:28 UTC_

Hardware Platforms:
Aplite, Basalt, Chalk, Diorite

Release Notes:
v6.10
 • Removed sleep function temporarily, until Pebble gets their act together and fixes the sleep bug
 • Add Korean language
 • Fixes that allow building on latest SDK

## Release-6.9
_Published on 2016-10-24 02:32:09 UTC_

Hardware Platforms:
Aplite, Basalt, Chalk, Diorite

Release Notes:
v6.9:
 	• New language option: 日本語 (Japanese, requires language pack)
	• New sidebar widget: heart rate monitor (for Pebble 2 only)
	• Improved LECO font metrics to better match "classic" appearance
	• Fixed a bug where antialiasing would fail to re-enable when changing fonts
	• Fixed a bug where certain combinations of widgets would overlap in Quick View mode

## Release-6.8
_Published on 2016-09-30 20:46:05 UTC_

Hardware Platforms:
Aplite, Basalt, Chalk, Diorite

Release Notes:
v6.8
 - Fixed an issue where the center widget wouldn't display if large fonts were enabled

## Release-6.7
_Published on 2016-09-30 02:19:43 UTC_

Hardware Platforms:
Aplite, Basalt, Chalk, Diorite

Release Notes:
v6.7
* Slight fix for off-center time on round watches

## Release-6.6
_Published on 2016-09-30 01:25:28 UTC_

Hardware Platforms:
Aplite, Basalt, Chalk, Diorite

Release Notes:
v6.6
- At last! Timeline Peek support!
- Improved font metrics for round watches
- Sidebar now defaults to grey on black and white watches
- Menu icon no longer has white corners on black and white watches

## Release-6.5
_Published on 2016-09-11 21:31:02 UTC_

Hardware Platforms:
Aplite, Basalt, Chalk, Diorite

Release Notes:
v6.5 "The calm before the storm"
- Preliminary support for the Pebble 2
- Added Gallacian language

## Release-6.4
_Published on 2016-06-22 02:42:14 UTC_

Hardware Platforms:
Aplite, Basalt, Chalk

Release Notes:
v6.4
 * Fixed issue from 6.3 update that broke GPS weather

## Release-6.3
_Published on 2016-06-20 01:19:52 UTC_

Hardware Platforms:
Aplite, Basalt, Chalk

Release Notes:
v6.3
 * Fixed issue where Weather Underground wouldn't work for certain locations

## Release-6.2
_Published on 2016-05-30 17:30:37 UTC_

Hardware Platforms:
Aplite, Basalt, Chalk

Release Notes:
v6.2
 - Fixed an issue where only "day" weather icons were shown if Weather Underground was used

## Release-6.1
_Published on 2016-05-29 14:50:51 UTC_

Hardware Platforms:
Aplite, Basalt, Chalk

Release Notes:
v6.1
 - Fix for potential issue where weather updates could become erroneously disabled

## Release-6.0
_Published on 2016-05-29 01:09:18 UTC_

Hardware Platforms:
Aplite, Basalt, Chalk

Release Notes:
	• New! Alternative weather data source
		○ Want more accurate weather data? Now you can choose Weather Underground, as long as you provide an API key!
	• Fixed a bug where OWM forecasts often had the same high and low temperature
	• Hourly vibe is now a double pulse, to distinguish it from notifications
	• Battery now shows "5%" instead of "0%" when at lowest level, to match official battery meter appearance
	• Percent sign now shows to the left of the number when language is set to Turkish
Month of "May" now fits slightly better in the sidebar

## Release-5.13
_Published on 2016-04-02 18:44:35 UTC_

Hardware Platforms:
Aplite, Basalt, Chalk

Release Notes:
v5.13
 - Fix for an issue where the weather failed to update (thanks, IguanaBen!)

## Release-5.12
_Published on 2016-03-28 14:27:00 UTC_

Hardware Platforms:
Aplite, Basalt, Chalk

Release Notes:
v5.12
 - Restored weather forecast functionality via dark magickry

## Release-5.11
_Published on 2016-03-25 04:01:36 UTC_

Hardware Platforms:
Aplite, Basalt, Chalk

Release Notes:
v5.11:
	• EMERGENCY WEATHER FIX: switched to OpenWeatherMap since Yahoo is finally dead for real
		○ Note: The weather forecast widget has been temporarily removed. It will return, in some form, in the future (OWM doesn't offer daily forecasts)
	• New default widgets since weather is evil
		○ Health on Time-series watches
		○ Battery with percentage on BW watches
	• Watchface loads and exits much faster, thanks to optimizations from tsymbaliuk!
	• Automatic battery indication threshold lowered to 10%
	• Health widget now respects global unit setting
New language: Welsh

## Release-5.10
_Published on 2016-03-10 03:45:01 UTC_

Hardware Platforms:
Aplite, Basalt, Chalk

Release Notes:
v5.10 - now with blatant disregard for decimal numbering systems
 - Ukranian language option (contributed by tsymbaliuk!)
 - Potential fix: parts of the sidebar should no longer disappear on round watches after receiving notifications (thanks to ygalanter for the code)
 - Potential fix: weather forecast icon should no longer disappear on BW pebbles
 - Day and month names no longer bump up against the edges of the sidebars on round watches

## Release-5.9
_Published on 2016-03-03 22:49:56 UTC_

Hardware Platforms:
Aplite, Basalt, Chalk

Release Notes:
v5.9
 - Fixed issue where step distances between 100m and 999m didn't show up
 - Added new user-contributed themes
 

## Release-5.8
_Published on 2016-03-03 22:31:33 UTC_

Hardware Platforms:
Aplite, Basalt, Chalk

Release Notes:
v5.8
 - Health now uses small font when showing distance above 10 kilometers
 - Battery levels less than 20% now display properly on BW watches w/ black sidebar

## Release-5.7
_Published on 2016-03-03 13:58:08 UTC_

Hardware Platforms:
Aplite, Basalt, Chalk

Release Notes:
v5.7
 - Half-hourly vibration works again

## Release-5.6
_Published on 2016-03-03 13:42:10 UTC_

Hardware Platforms:
Aplite, Basalt, Chalk

Release Notes:
v5.6
 - Fixes to Indonesian language

## Release-5.5
_Published on 2016-03-01 13:54:28 UTC_

Hardware Platforms:
Aplite, Basalt, Chalk

Release Notes:
v5.5 
 - Fixed bug where long month names wouldn't display correctly in large sidebar mode

## Release-5.4
_Published on 2016-02-27 23:43:40 UTC_

Hardware Platforms:
Aplite, Basalt, Chalk

Release Notes:
v5.4
* Minor fix: Restful and total sleep options are no longer backwards

## Release-5.3
_Published on 2016-02-23 21:16:49 UTC_

Hardware Platforms:
Aplite, Basalt, Chalk

Release Notes:
v5.3
	• Tweaks to Serbian language (contributed by siklosi)
	• Fixed problem where the day was out of sync with reality near midnight
	• Fixed problem where alt-tz complication was like 5 hours off
	• Fix a bug where the month wouldn't display in Russian

## Release-5.2
_Published on 2016-02-23 02:04:42 UTC_

Hardware Platforms:
Aplite, Basalt, Chalk

Release Notes:
v5.2
 * Reduced memory footprint to fix constant crashes on classic Pebbles

## Release-5.1
_Published on 2016-02-22 05:40:55 UTC_

Hardware Platforms:
Aplite, Basalt, Chalk

Release Notes:
v5.1
	• New languages: Chinese and Indonesian
	• Added complication for SWATCH INTERNET TIME, the time measurement of the FUTURE
	• Option to disable automatic low battery indication
	• Alt-TZ complication now shows am/pm if watch is in 12-hour mode (thanks to notlaforge for the code!)
	• Added health accessibility checks to placate Katharine先輩

## Release-5.0
_Published on 2016-02-15 14:32:01 UTC_

Hardware Platforms:
Aplite, Basalt, Chalk

Release Notes:
v5.0!
• New: Bold font options!
• New: Health complication for sidebar, with options for distance and sleep type!
• New: Automatic battery status indication when battery is low or charging
• Improved: Inverted sidebar icons now take on the color of sidebar text
• Fixed: Battery charge icon displays correctly
• Fixed: Issue with GPS weather
• 4 new languages: Croatian, Irish, Latvian, and Serbian
• 12 exciting new user-contributed themes!

## Release-4.8
_Published on 2016-02-09 21:40:00 UTC_

Hardware Platforms:
Aplite, Basalt, Chalk

Release Notes:
v4.8
 - User contribution: "Current Time" widget by Jajunk
 - New languages: Irish and Croatian

## Release-4.7
_Published on 2016-02-05 04:29:07 UTC_

Hardware Platforms:
Aplite, Basalt, Chalk

Release Notes:
v4.7
- New: Grey support for aplite users, so now you get 15 themes instead of 4!
- New: Super secret bonus feature for color watches: set the sidebar to black and get fancy inverted colors

## Release-4.6
_Published on 2016-01-22 02:13:49 UTC_

Hardware Platforms:
Aplite, Basalt, Chalk

Release Notes:
v4.6
- FIX: Yahoo weather now updates again
- IMPROVEMENT: Automatic locations are now cached which should theoretically improve weather reliability

## Release-4.5
_Published on 2015-12-19 14:37:57 UTC_

Hardware Platforms:
Aplite, Basalt, Chalk

Release Notes:
v4.5:
 - Added Hungarian language
 - Added Slovenian language
 - Possible fix for half-hourly vibe issue
 - 4 new themes! 

## Release-4.4
_Published on 2015-11-18 01:35:12 UTC_

Hardware Platforms:
Aplite, Basalt, Chalk

Release Notes:
v4.4
 - Added Lithuanian language

## Release-4.3
_Published on 2015-10-24 14:20:02 UTC_

Hardware Platforms:
Aplite, Basalt, Chalk

Release Notes:
v4.3:
- Fixed: bug where the week number abbreviation was wrong for certain lanugages

## Release-4.2
_Published on 2015-10-24 12:34:05 UTC_

Hardware Platforms:
Aplite, Basalt, Chalk

Release Notes:
v4.2:
- Config: New preset selector design that doesn't take up so much vertical space
- Config: Several new exciting presets based on user submissions!
- New: Danish language option

## Release-4.1
_Published on 2015-10-13 13:02:15 UTC_

Hardware Platforms:
Aplite, Basalt, Chalk

Release Notes:
v4.1:
* Fixed: Week number is no longer off by one
* Fixed: Date widget now draws correctly incorrectly on Aplite when large fonts are enabled
* Fixed: Only show battery when low setting now works again.

## Release-4.0
_Published on 2015-10-10 02:18:33 UTC_

Hardware Platforms:
Aplite, Basalt, Chalk

Release Notes:
v4.0 -- The craziest release yet!
* New: Customize order and contents of the sidebar!
* New: Alternate time zone
* New: Weather forecast
* New: Week number
* New: Seconds
* New: Support for the new Pebble Time Round!
* Improved: Sidebar settings now show live preview of selected complications


## Release-3.7
_Published on 2015-09-14 23:40:11 UTC_

Hardware Platforms:
Aplite, Basalt

Release Notes:
v3.7
* Added Finnish support (thanks, samuelmr!)

## Release-3.6
_Published on 2015-09-13 17:05:24 UTC_

Hardware Platforms:
Aplite, Basalt

Release Notes:
v3.6
* Weather refreshes on bluetooth reconnection
* Potential fix for the randomly-resetting-settings issue

## Release-3.5
_Published on 2015-09-04 12:24:58 UTC_

Hardware Platforms:
Aplite, Basalt

Release Notes:
v3.5:
* Added Basque language (thanks, alaintxu!)
* More elaborate vibration pattern on BT disconnect (thanks, pjexposito!)

## Release-3.4
_Published on 2015-09-01 22:42:38 UTC_

Hardware Platforms:
Aplite, Basalt

Release Notes:
v3.4:
* Added Russian and Estonian languages

## Release-3.3
_Published on 2015-08-31 22:11:52 UTC_

Hardware Platforms:
Aplite, Basalt

Release Notes:
v3.3:
* Settings page now automatically populates with default settings for new users
* Norwegian language support

## Release-3.2
_Published on 2015-08-27 01:26:39 UTC_

Hardware Platforms:
Aplite, Basalt

Release Notes:
v3.2:
 * Catalan language support
 * Added 4 user-contributed themes (by Joe, Joel, John, and Kazoon18)


## Release-3.1
_Published on 2015-08-24 02:44:29 UTC_

Hardware Platforms:
Aplite, Basalt

Release Notes:
v3.1:
* Improvement to Polish translation
* Theme previews use colors more like what the Pebble Time screen actually shows

## Release-3.0
_Published on 2015-08-24 02:32:57 UTC_

Hardware Platforms:
Aplite, Basalt

Release Notes:
V3.0
	• New: Ability to save and load custom themes
	• New: Ability to share custom themes or suggest them as presets
	• New: Large font option for the sidebar
	• New: Option to show the battery indicator only when low
	• New: Language options for Vietnamese and Romanian
	• Improved: Settings page now remembers last-selected colors and settings
	• Improved: Color pickers now show colors in more logical order

## Release-2.8
_Published on 2015-08-01 16:31:42 UTC_

Hardware Platforms:
Aplite, Basalt

Release Notes:
v2.8:
* Added Slovak language

## Release-2.7
_Published on 2015-07-30 07:52:59 UTC_

Hardware Platforms:
Aplite, Basalt

Release Notes:
v2.7:
Added: Swedish and Polish language options

## Release-2.6
_Published on 2015-07-26 22:18:07 UTC_

Hardware Platforms:
Aplite, Basalt

Release Notes:
v2.6
* Another potential fix for iOS location bug

## Release-2.5
_Published on 2015-07-25 13:59:52 UTC_

Hardware Platforms:
Aplite, Basalt

Release Notes:
v2.5
 * Potential fix for localStorage on iOS

## Release-2.4
_Published on 2015-07-24 16:35:16 UTC_

Hardware Platforms:
Aplite, Basalt

Release Notes:
v2.4
* New: Option to vibrate every 30min

## Release-2.3
_Published on 2015-07-24 15:10:33 UTC_

Hardware Platforms:
Aplite, Basalt

Release Notes:
v2.3:
* Change: Switched weather data source to Yahoo weather, for hopefully improved accuracy
* Fixed: Battery percentage no longer shows when charging, since numbers were wildly inaccurate
* Fixed: Hourly vibration is now actually hourly
* Fixed: Issue where weather would not appear on first run

## Release-2.2
_Published on 2015-07-23 16:54:37 UTC_

Hardware Platforms:
Aplite, Basalt

Release Notes:
v2.2:
 * Fixed: Language can now be set back to English after being changed

## Release-2.1
_Published on 2015-07-23 16:10:29 UTC_

Hardware Platforms:
Aplite, Basalt

Release Notes:
v2.1:
* Fixed: Battery meter is no longer slightly off-center when weather is enabled

## Release-2.0
_Published on 2015-07-23 15:48:35 UTC_

Hardware Platforms:
Aplite, Basalt

Release Notes:
v2.0:
 * New: option to show battery charge as percent
 * New: option to disable weather entirely
 * New: option for hourly bluetooth vibration
 * New: alternate font option, LECO (thanks to MatthewReiss for bitmaps)
 * New: language options for Dutch, Turkish, Czech, Portuguese, and Greek (language pack required for Greek) 
 * New: Watchface selection menu icon
 * Improved: Icons on Pebble Time watches now render more accurately
 * Improved: Battery icon now indicates when watch is charging
 * Fixed: Potential crash on poor internet connections

## Release-1.8
_Published on 2015-07-20 14:48:52 UTC_

Hardware Platforms:
Aplite, Basalt

Release Notes:
v1.8: Fixed battery indicator problem on OG pebble with dark themes

## Release-1.7
_Published on 2015-07-20 11:00:49 UTC_

Hardware Platforms:
Aplite, Basalt

Release Notes:
v1.7: Fixed issue with OpenWeatherMap API call

## Release-1.6
_Published on 2015-07-19 22:45:12 UTC_

Hardware Platforms:
Aplite, Basalt

Release Notes:
v1.6: Fixed bug with day names in italian

## Release-1.5
_Published on 2015-07-19 20:24:55 UTC_

Hardware Platforms:
Aplite, Basalt

Release Notes:
v1.5:
* Adds option for battery indicator
* Adds option for display of leading zero in the time

## Release-1.4
_Published on 2015-07-19 13:47:01 UTC_

Hardware Platforms:
Aplite, Basalt

Release Notes:
v1.4:
* Support for original Pebble and Pebble Steel
* Sidebar text color is now customizable
* 5 different language options for months and days

## Release-1.2
_Published on 2015-07-17 17:46:28 UTC_

Hardware Platforms:
Basalt

Release Notes:
v1.2
* Weather location can now be set manually
* Option to vibrate on bluetooth disconnection


## Release-1.1
_Published on 2015-07-16 01:32:41 UTC_

Hardware Platforms:
Basalt

Release Notes:
v1.1: Bugfix: now displays 100+ degree temperatures properly

## Release-1.0
_Published on 2015-07-15 13:45:49 UTC_

Hardware Platforms:
Basalt

Release Notes:
v1.0 - Initial release

