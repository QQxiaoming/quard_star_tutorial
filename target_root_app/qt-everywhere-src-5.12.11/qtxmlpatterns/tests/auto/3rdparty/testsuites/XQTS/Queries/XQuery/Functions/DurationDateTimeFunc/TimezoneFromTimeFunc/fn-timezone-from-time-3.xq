(:*******************************************************:)
(:Test: fn-timezone-from-time-3                          :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 27, 2005                                    :)
(:Purpose: Evaluates The "timezone-from-time" function   :)
(:as part of a numeric greater than operation (gt operator):)
(:*******************************************************:)

fn:timezone-from-time(xs:time("01:23:02Z")) gt fn:timezone-from-time(xs:time("01:23:03Z"))