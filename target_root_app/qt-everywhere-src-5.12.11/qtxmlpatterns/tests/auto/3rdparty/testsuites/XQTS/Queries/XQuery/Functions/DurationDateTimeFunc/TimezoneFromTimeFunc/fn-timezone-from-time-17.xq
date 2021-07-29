(:*******************************************************:)
(:Test: fn-timezone-from-time-17                         :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 27, 2005                                    :)
(:Purpose: Evaluates The "timezone-from-time" function   :)
(:as part of a "numeric-equal" expression (ne operator)  :) 
(:*******************************************************:)

fn:timezone-from-time(xs:time("10:00:01Z")) ne fn:timezone-from-time(xs:time("01:01:00Z"))
