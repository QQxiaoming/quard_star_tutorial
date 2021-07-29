(:*******************************************************:)
(:Test: seconds-from-dateTime-8                          :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 6, 2005                                     :)
(:Purpose: Evaluates The "seconds-from-dateTime" function:)
(:as part of a "+" expression.                           :) 
(:*******************************************************:)

fn:seconds-from-dateTime(xs:dateTime("1970-01-01T00:00:10Z")) + fn:seconds-from-dateTime(xs:dateTime("1970-01-01T10:00:11Z"))
