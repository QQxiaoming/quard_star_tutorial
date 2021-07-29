(:*******************************************************:)
(:Test: hours-from-time-13                               :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 6, 2005                                     :)
(:Purpose: Evaluates The "hours-from-time" function      :)
(:as part of a "mod" expression.                         :) 
(:*******************************************************:)

fn:hours-from-time(xs:time("10:00:00Z")) mod fn:hours-from-time(xs:time("03:00:00Z"))
