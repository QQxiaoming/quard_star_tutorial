(:*******************************************************:)
(:Test: hours-from-time-11                               :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 6, 2005                                     :)
(:Purpose: Evaluates The "hours-from-time" function      :)
(:as part of a "div" expression.                         :) 
(:*******************************************************:)

fn:hours-from-time(xs:time("22:00:00Z")) div fn:hours-from-time(xs:time("02:00:00Z"))
