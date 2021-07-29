(:*******************************************************:)
(:Test: fn-month-from-dateTime-16                        :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 8, 2005                                     :)
(:Purpose: Evaluates The "month-from-dateTime" function  :)
(:as part of a "numeric-equal" expression (eq operator)  :) 
(:*******************************************************:)

fn:month-from-dateTime(xs:dateTime("1970-01-01T10:00:00Z")) eq fn:month-from-dateTime(xs:dateTime("1970-01-01T10:00:00Z"))
