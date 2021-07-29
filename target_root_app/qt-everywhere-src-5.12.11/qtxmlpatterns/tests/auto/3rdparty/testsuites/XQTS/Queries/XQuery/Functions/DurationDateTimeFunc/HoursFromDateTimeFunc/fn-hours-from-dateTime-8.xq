(:*******************************************************:)
(:Test: hours-from-dateTime-8                            :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 6, 2005                                     :)
(:Purpose: Evaluates The "hours-from-dateTime" function  :)
(:as part of a "+" expression.                           :) 
(:*******************************************************:)

fn:hours-from-dateTime(xs:dateTime("1970-01-01T00:00:00Z")) + fn:hours-from-dateTime(xs:dateTime("1970-01-01T10:00:00Z"))
