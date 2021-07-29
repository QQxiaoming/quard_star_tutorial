(:*******************************************************:)
(:Test: seconds-from-time-2                              :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 6, 2005                                     :)
(:Purpose: Evaluates The "seconds-from-time" function    :)
(:used as part of a numeric less than expression (lt operator) :)
(:*******************************************************:)

fn:seconds-from-time(xs:time("21:23:04Z")) lt fn:seconds-from-time(xs:time("21:24:00Z"))