(:*******************************************************:)
(:Test: hours-from-time-10                               :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 6, 2005                                     :)
(:Purpose: Evaluates The "hours-from-time" function      :)
(:as part of a "*" expression.                           :) 
(:*******************************************************:)

fn:hours-from-time(xs:time("02:00:00Z")) * fn:hours-from-time(xs:time("10:00:00Z"))
