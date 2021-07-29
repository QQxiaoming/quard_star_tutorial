(:*******************************************************:)
(:Test: fn-timezone-from-dateTime-17                     :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 27, 2005                                     :)
(:Purpose: Evaluates The "timezone-from-dateTime" function  :)
(:as part of a "numeric-equal" expression (ne operator)  :) 
(:*******************************************************:)

fn:timezone-from-dateTime(xs:dateTime("1970-01-01T10:00:00Z")) ne fn:timezone-from-dateTime(xs:dateTime("1970-01-01T10:00:00Z"))
