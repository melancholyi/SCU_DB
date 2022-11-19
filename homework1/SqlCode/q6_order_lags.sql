SELECT Id,preOrderDate,
				ROUND(JULIANDAY(OrderDate) - JULIANDAY(preOrderDate) , 2)
FROM(
	SELECT  Id, OrderDate , 
					LAG(OrderDate , 1 , OrderDate) OVER (ORDER BY OrderDate ASC) AS preOrderDate
	FROM 'Order'
	WHERE CustomerId = 'BLONP'
	LIMIT 10
);