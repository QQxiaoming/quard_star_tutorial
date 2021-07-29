(:*******************************************************:)
(:Test: minutes-from-dateTime-12                         :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 6, 2005                                     :)
(:Purpose: Evaluates The "minutes-from-dateTime" function:)
(:as part of a "idiv" expression.                        :) 
(:*******************************************************:)

fn:minutes-from-dateTime(xs:dateTime("1970-01-01T10:10:00Z")) idiv fn:minutes-from-dateTime(xs:dateTime("1970-01-01T02:05:00Z"))
