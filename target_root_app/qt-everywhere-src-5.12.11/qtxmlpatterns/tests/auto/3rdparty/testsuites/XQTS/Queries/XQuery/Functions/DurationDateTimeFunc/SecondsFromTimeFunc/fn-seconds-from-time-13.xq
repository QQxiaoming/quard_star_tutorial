(:*******************************************************:)
(:Test: seconds-from-time-13                             :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 6, 2005                                     :)
(:Purpose: Evaluates The "seconds-from-time" function    :)
(:as part of a "mod" expression.                         :) 
(:*******************************************************:)

fn:seconds-from-time(xs:time("10:10:20Z")) mod fn:seconds-from-time(xs:time("03:03:02Z"))
