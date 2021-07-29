(:*******************************************************:)
(:Test: fn-timezone-from-time-19                         :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 27, 2005                                    :)
(:Purpose: Evaluates The "timezone-from-time" function   :)
(:as part of a "numeric-equal" expression (ge operator)  :) 
(:*******************************************************:)

fn:timezone-from-time(xs:time("10:03:01Z")) ge fn:timezone-from-time(xs:time("10:04:02Z"))
