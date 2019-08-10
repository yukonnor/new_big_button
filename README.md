# new_big_button
Refactored Code &amp; Documentation for Look Mum No Computer's Big Button. See his original post here: https://www.lookmumnocomputer.com/big-button

## background
While playing with my synth, I thought it'd be handy if I could create rhythms by recording button taps. The tap pattern would then be looped in a sequence. While looking around for inspiration / existing hardware that accomplished this, I stumbled upon Look Mom No Computer's "Big Button". Woo!

## updates
I went through LMNC's code and refactored it a bit to clean it up (it now only takes up 11% of my nano's program memory) and to make it easier for me to understand. I also added comments to help me understand what was taking place. This file is named "BIG_BUTTON_2.ino". 

I am in no way a professional developer (just started to play around with Arduino a couple of years ago), so sorry if I've committed any code faux pas. 

## simplified version

I also created an alternaitve version that gets rid of some of the features and adds some others. 
- It goes from 6 ouputs to 4 (I only have so many things to trigger)
- It scraps Shift and Delete (I found that while using, I only used "Clear")
- It adds a bank LED to let you know which bank you're on for each channel.
- It adds a 'Beginning of Cycle' LED and output to let you know when the sequence restarts. This can be used to trigger external functions.


