(:*******************************************************:)
(:Test: minutes-from-time-4                              :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 6, 2005                                     :)
(:Purpose: Evaluates The "minutes-from-time" function    :)
(:used as an argument to an avg function.                :)
(:*******************************************************:)

fn:avg((fn:minutes-from-time(xs:time("01:10:00Z")), fn:minutes-from-time(xs:time("01:20:00Z"))))