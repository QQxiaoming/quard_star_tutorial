(:*******************************************************:)
(:Test: minutes-from-dateTime-13                         :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 6, 2005                                     :)
(:Purpose: Evaluates The "minutes-from-dateTime" function:)
(:as part of a "mod" expression.                         :) 
(:*******************************************************:)

fn:minutes-from-dateTime(xs:dateTime("1970-01-01T10:10:00Z")) mod fn:minutes-from-dateTime(xs:dateTime("1970-01-01T03:10:00Z"))
