(:*******************************************************:)
(:Test: seconds-from-dateTime-11                         :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 6, 2005                                     :)
(:Purpose: Evaluates The "seconds-from-dateTime" function:)
(:as part of a "div" expression.                         :) 
(:*******************************************************:)

fn:seconds-from-dateTime(xs:dateTime("1970-01-01T02:10:10Z")) div fn:seconds-from-dateTime(xs:dateTime("1970-01-01T10:05:05Z"))
