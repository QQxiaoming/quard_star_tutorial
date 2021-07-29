(:*******************************************************:)
(:Test: fn-timezone-from-time-14                         :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 27, 2005                                    :)
(:Purpose: Evaluates The "timezone-from-time" function   :)
(:as an argument to the "fn:string" function.            :) 
(:*******************************************************:)

fn:string(fn:timezone-from-time(xs:time("10:00:01Z")))
