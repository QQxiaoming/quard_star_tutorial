(:*******************************************************:)
(:Test: hours-from-time-12                               :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 6, 2005                                     :)
(:Purpose: Evaluates The "hours-from-time" function      :)
(:as part of a "idiv" expression.                        :) 
(:*******************************************************:)

fn:hours-from-time(xs:time("10:00:00Z")) idiv fn:hours-from-time(xs:time("02:00:00Z"))
