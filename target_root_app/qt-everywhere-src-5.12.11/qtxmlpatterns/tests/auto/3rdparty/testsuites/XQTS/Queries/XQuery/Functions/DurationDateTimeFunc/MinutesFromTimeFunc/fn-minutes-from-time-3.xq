(:*******************************************************:)
(:Test: minutes-from-time-3                              :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 6, 2005                                     :)
(:Purpose: Evaluates The "minutes-from-time" function    :)
(:as part of a numeric greater than operation (gt operator):)
(:*******************************************************:)

fn:minutes-from-time(xs:time("01:23:00Z")) gt fn:minutes-from-time(xs:time("01:23:00Z"))