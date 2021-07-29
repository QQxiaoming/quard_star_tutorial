(:*******************************************************:)
(:Test: hours-from-time-17                               :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 6, 2005                                     :)
(:Purpose: Evaluates The "hours-from-time" function      :)
(:as part of a "numeric-equal" expression (ne operator)  :) 
(:*******************************************************:)

fn:hours-from-time(xs:time("10:00:00Z")) ne fn:hours-from-time(xs:time("01:00:00Z"))
