(:*******************************************************:)
(:Test: fn-timezone-from-time-18                         :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 27, 2005                                    :)
(:Purpose: Evaluates The "timezone-from-time" function   :)
(:as part of a "numeric-equal" expression (le operator)  :) 
(:*******************************************************:)

fn:timezone-from-time(xs:time("10:00:00Z")) le fn:timezone-from-time(xs:time("10:00:00Z"))
