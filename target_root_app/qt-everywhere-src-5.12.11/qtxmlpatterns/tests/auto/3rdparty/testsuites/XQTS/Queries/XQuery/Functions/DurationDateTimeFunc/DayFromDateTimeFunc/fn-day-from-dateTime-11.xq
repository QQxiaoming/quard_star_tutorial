(:*******************************************************:)
(:Test: fn-day-from-dateTime-11                          :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 8, 2005                                     :)
(:Purpose: Evaluates The "day-from-dateTime" function    :)
(:as part of a "div" expression.                         :) 
(:*******************************************************:)

fn:day-from-dateTime(xs:dateTime("1000-01-10T02:00:00Z")) div fn:day-from-dateTime(xs:dateTime("0050-01-05T10:00:00Z"))
