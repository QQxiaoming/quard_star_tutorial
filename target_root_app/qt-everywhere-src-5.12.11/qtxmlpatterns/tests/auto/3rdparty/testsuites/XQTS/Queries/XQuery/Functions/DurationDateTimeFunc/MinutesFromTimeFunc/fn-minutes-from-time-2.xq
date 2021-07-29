(:*******************************************************:)
(:Test: minutes-from-time-2                              :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 6, 2005                                     :)
(:Purpose: Evaluates The "minutes-from-time" function    :)
(:used as part of a numeric less than expression (lt operator) :)
(:*******************************************************:)

fn:minutes-from-time(xs:time("21:23:00Z")) lt fn:minutes-from-time(xs:time("21:24:00Z"))