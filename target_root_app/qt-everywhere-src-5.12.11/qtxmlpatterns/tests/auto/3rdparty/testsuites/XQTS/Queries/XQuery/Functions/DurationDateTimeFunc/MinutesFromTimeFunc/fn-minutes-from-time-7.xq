(:*******************************************************:)
(:Test: minutes-from-time-7                              :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 6, 2005                                     :)
(:Purpose: Evaluates The "minutes-from-time" function    :)
(:used as part of an abs function.                       :) 
(:*******************************************************:)

fn:abs(fn:minutes-from-time(xs:time("23:20:00Z")))
