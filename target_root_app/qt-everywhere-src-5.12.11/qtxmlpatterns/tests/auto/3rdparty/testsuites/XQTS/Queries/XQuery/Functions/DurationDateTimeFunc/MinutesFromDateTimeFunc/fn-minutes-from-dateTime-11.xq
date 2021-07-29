(:*******************************************************:)
(:Test: minutes-from-dateTime-11                         :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 6, 2005                                     :)
(:Purpose: Evaluates The "minutes-from-dateTime" function:)
(:as part of a "div" expression.                         :) 
(:*******************************************************:)

fn:minutes-from-dateTime(xs:dateTime("1970-01-01T02:10:00Z")) div fn:minutes-from-dateTime(xs:dateTime("1970-01-01T10:05:00Z"))
