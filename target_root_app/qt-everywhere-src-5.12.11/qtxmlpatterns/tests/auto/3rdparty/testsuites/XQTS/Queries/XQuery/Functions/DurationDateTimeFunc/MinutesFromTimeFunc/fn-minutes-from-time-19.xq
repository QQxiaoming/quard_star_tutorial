(:*******************************************************:)
(:Test: minutes-from-time-19                             :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 6, 2005                                     :)
(:Purpose: Evaluates The "minutes-from-time" function    :)
(:as part of a "numeric-equal" expression (ge operator)  :) 
(:*******************************************************:)

fn:minutes-from-time(xs:time("10:03:00Z")) ge fn:minutes-from-time(xs:time("10:04:00Z"))
