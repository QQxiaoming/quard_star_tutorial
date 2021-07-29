(:*******************************************************:)
(:Test: seconds-from-time-3                              :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 6, 2005                                     :)
(:Purpose: Evaluates The "seconds-from-time" function    :)
(:as part of a numeric greater than operation (gt operator):)
(:*******************************************************:)

fn:seconds-from-time(xs:time("01:23:02Z")) gt fn:seconds-from-time(xs:time("01:23:03Z"))