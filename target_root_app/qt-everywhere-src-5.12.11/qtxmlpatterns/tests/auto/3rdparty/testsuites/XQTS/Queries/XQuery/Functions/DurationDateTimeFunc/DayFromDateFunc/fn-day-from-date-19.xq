(:*******************************************************:)
(:Test: day-from-date-19                                 :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 6, 2005                                     :)
(:Purpose: Evaluates The "day-from-date" function        :)
(:as part of a "numeric-equal" expression (ge operator)  :) 
(:*******************************************************:)

fn:day-from-date(xs:date("1970-01-03Z")) ge fn:day-from-date(xs:date("1970-01-01Z"))