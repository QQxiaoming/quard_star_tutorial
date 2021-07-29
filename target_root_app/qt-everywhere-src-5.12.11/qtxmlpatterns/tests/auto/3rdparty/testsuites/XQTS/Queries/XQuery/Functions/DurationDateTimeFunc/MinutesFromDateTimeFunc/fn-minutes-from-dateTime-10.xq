(:*******************************************************:)
(:Test: minutes-from-dateTime-10                         :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 6, 2005                                     :)
(:Purpose: Evaluates The "minutes-from-dateTime" function:)
(:as part of a "*" expression.                           :) 
(:*******************************************************:)

fn:minutes-from-dateTime(xs:dateTime("1970-01-01T02:02:00Z")) * fn:minutes-from-dateTime(xs:dateTime("1970-01-01T10:03:00Z"))
