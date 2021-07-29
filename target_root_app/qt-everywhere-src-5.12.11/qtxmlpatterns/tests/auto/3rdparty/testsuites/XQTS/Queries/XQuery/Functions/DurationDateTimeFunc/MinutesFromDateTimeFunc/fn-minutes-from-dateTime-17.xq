(:*******************************************************:)
(:Test: minutes-from-dateTime-17                         :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 6, 2005                                     :)
(:Purpose: Evaluates The "minutes-from-dateTime" function:)
(:as part of a "numeric-equal" expression (ne operator)  :) 
(:*******************************************************:)

fn:minutes-from-dateTime(xs:dateTime("1970-01-01T10:10:00Z")) ne fn:minutes-from-dateTime(xs:dateTime("1970-01-01T10:10:00Z"))
