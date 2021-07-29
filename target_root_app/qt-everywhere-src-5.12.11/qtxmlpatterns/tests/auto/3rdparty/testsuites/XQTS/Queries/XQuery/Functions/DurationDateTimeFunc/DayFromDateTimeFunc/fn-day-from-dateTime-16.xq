(:*******************************************************:)
(:Test: fn-day-from-dateTime-16                          :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 8, 2005                                     :)
(:Purpose: Evaluates The "day-from-dateTime" function   :)
(:as part of a "numeric-equal" expression (eq operator)  :) 
(:*******************************************************:)

fn:day-from-dateTime(xs:dateTime("1970-01-01T10:00:00Z")) eq fn:day-from-dateTime(xs:dateTime("1970-01-02T10:00:00Z"))
