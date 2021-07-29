(:*******************************************************:)
(:Test: fn-timezone-from-time-4                          :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 27, 2005                                    :)
(:Purpose: Evaluates The "timezone-from-time" function as :)
(:part of a numeric-less-than expression (le operator)   :)
(:*******************************************************:)

fn:timezone-from-time(xs:time("01:10:20Z")) le fn:timezone-from-time(xs:time("01:20:30Z"))