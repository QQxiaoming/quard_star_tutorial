(:*******************************************************:)
(:Test: fn-timezone-from-time-16                         :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 27, 2005                                    :)
(:Purpose: Evaluates The "timezone-from-time" function   :)
(:as part of a "numeric-equal" expression (eq operator)  :) 
(:*******************************************************:)

fn:timezone-from-time(xs:time("10:02:01Z")) eq fn:timezone-from-time(xs:time("10:02:00Z"))
