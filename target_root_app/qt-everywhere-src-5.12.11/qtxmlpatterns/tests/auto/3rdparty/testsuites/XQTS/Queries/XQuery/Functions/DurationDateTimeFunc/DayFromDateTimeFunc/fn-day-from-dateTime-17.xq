(:*******************************************************:)
(:Test: fn-day-from-dateTime-17                          :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 6, 2005                                     :)
(:Purpose: Evaluates The "day-from-dateTime" function    :)
(:as part of a "numeric-equal" expression (ne operator)  :) 
(:*******************************************************:)

fn:day-from-dateTime(xs:dateTime("1970-01-02T10:00:00Z")) ne fn:day-from-dateTime(xs:dateTime("1970-02-02T10:00:00Z"))
