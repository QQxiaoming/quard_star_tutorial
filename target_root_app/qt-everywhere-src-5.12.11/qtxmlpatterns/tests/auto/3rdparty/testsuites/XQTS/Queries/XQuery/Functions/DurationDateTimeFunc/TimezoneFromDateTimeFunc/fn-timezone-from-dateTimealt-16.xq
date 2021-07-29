(:*******************************************************:)
(:Test: fn-timezone-from-dateTime-16                     :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 27, 2005                                    :)
(:Purpose: Evaluates The "timezone-from-dateTime" function  :)
(:as part of a "numeric-equal" expression (eq operator)  :) 
(:*******************************************************:)

fn:timezone-from-dateTime(xs:dateTime("1970-01-01T10:00:00Z")) eq fn:timezone-from-dateTime(xs:dateTime("1970-01-01T10:00:00Z"))
