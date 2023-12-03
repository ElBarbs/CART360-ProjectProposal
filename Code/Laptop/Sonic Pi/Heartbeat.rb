BPM = 30

ANXIETY = 0

# Values for each anxiety level.
D2 = [2, 1.5, 1.25, 1, 0.75, 0.5, 0.25, 0.125]
D3 = [0.5, 0.25, 0.25, 0.25, 0.15, 0.1, 0.075, 0.075]

# Note, release, amp and sleep duration.
NOTES = [
  [:D2, 0.1, 1.5, D2[ANXIETY]],
  [:D3, 0, 0.8, D3[ANXIETY]]
]

live_loop :main do
  ANXIETY, _ = sync "/osc*/anxiety"
    
  # Note, release, amp and sleep duration.
  NOTES = [
    [:D2, 0.1, 1.5, D2[ANXIETY]],
    [:D3, 0, 0.8, D3[ANXIETY]]
  ]
end

live_loop :heartbeat do
  with_fx :lpf do
    with_fx :bitcrusher, bits: 4, sample_rate: 2000 do
      with_fx :gverb, room: 1, spread: 1, tail_level: 0.1, ref_level: 0.2 do
        NOTES.each do |note, release, amp, sleep_duration|
          play note, release: release, amp: amp
          sleep sleep_duration
        end
      end
    end
  end
end

live_loop :dark_amb do
  use_synth :dark_ambience
  with_fx :krush, res: 0.5 do
    if ANXIETY >= 2
      play :D2, amp: 1.25, sustain: 2
    end
    sleep 8
  end
end

live_loop :scratch do
  if ANXIETY >= 4
    with_fx :tremolo do
      sample :vinyl_scratch, amp: 1.25, rate: 0.5
    end
  end
  sleep 1
end

live_loop :noise do
  use_synth :bnoise
  if ANXIETY == 6
    play :B3, release: 0.1
  end
  sleep 0.5
end