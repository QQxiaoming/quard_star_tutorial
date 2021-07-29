(:*******************************************************:)
(:Test: fn-timezone-from-time-7                          :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 27, 2005                                    :)
(:Purpose: Evaluates The "timezone-from-time" function   :)
(: with a timezone component of "-00:00".                :) 
(:*******************************************************:)

fn:timezone-from-time(xs:time("23:20:59-00:00"))
