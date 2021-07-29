(:*******************************************************:)
(:Test: fn-day-from-dateTime-13                          :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 8, 2005                                     :)
(:Purpose: Evaluates The "day-from-dateTime" function    :)
(:as part of a "mod" expression.                         :) 
(:*******************************************************:)

fn:day-from-dateTime(xs:dateTime("1970-01-10T10:00:00Z")) mod fn:day-from-dateTime(xs:dateTime("1970-01-03T03:00:00Z"))
