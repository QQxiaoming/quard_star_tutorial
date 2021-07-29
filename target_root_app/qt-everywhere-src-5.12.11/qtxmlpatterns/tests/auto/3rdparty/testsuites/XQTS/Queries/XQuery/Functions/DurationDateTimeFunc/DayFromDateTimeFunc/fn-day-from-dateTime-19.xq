(:*******************************************************:)
(:Test: fn-day-from-dateTime-19                          :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 8, 2005                                     :)
(:Purpose: Evaluates The "day-from-dateTime" function    :)
(:as part of a "numeric-equal" expression (ge operator)  :) 
(:*******************************************************:)

fn:day-from-dateTime(xs:dateTime("1971-01-10T10:00:00Z")) ge fn:day-from-dateTime(xs:dateTime("1970-01-11T10:00:00Z"))
