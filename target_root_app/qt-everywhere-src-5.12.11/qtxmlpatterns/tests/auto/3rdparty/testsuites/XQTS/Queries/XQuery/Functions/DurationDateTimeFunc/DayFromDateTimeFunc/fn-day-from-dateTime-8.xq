(:*******************************************************:)
(:Test: fn-day-from-dateTime-8                           :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 8, 2005                                     :)
(:Purpose: Evaluates The "day-from-dateTime" function    :)
(:as part of a "+" expression.                           :) 
(:*******************************************************:)

fn:day-from-dateTime(xs:dateTime("1970-01-03T00:00:00Z")) + fn:day-from-dateTime(xs:dateTime("1970-01-01T10:00:00Z"))
