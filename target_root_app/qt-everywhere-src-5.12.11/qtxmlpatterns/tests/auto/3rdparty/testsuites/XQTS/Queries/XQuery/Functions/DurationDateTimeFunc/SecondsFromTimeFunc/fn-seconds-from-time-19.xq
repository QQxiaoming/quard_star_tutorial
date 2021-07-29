(:*******************************************************:)
(:Test: seconds-from-time-19                             :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 6, 2005                                     :)
(:Purpose: Evaluates The "seconds-from-time" function    :)
(:as part of a "numeric-equal" expression (ge operator)  :) 
(:*******************************************************:)

fn:seconds-from-time(xs:time("10:03:01Z")) ge fn:seconds-from-time(xs:time("10:04:02Z"))
