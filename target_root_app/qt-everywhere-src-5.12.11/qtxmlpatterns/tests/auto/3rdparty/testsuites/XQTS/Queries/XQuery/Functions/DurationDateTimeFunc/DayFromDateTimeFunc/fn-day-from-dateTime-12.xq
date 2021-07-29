(:*******************************************************:)
(:Test: fn-day-from-dateTime-12                          :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 8, 2005                                     :)
(:Purpose: Evaluates The "day-from-dateTime" function    :)
(:as part of a "idiv" expression.                        :) 
(:*******************************************************:)

fn:day-from-dateTime(xs:dateTime("1970-01-10T10:00:00Z")) idiv fn:day-from-dateTime(xs:dateTime("1970-01-10T02:00:00Z"))
