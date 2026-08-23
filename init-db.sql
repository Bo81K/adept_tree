CREATE TABLE IF NOT EXISTS tree_nodes (
    id SERIAL PRIMARY KEY,
    parent_id INTEGER REFERENCES tree_nodes(id) ON DELETE CASCADE,
    name TEXT NOT NULL,
    value DOUBLE PRECISION NULL,
    is_leaf BOOLEAN NOT NULL DEFAULT FALSE,

    CONSTRAINT tree_nodes_name_not_empty
        CHECK (length(trim(name)) > 0),

    CONSTRAINT tree_nodes_value_consistency
        CHECK (
	    (is_leaf = TRUE AND value IS NOT NULL)
	    OR (is_leaf = FALSE AND value IS NULL)
	)
);



CREATE INDEX IF NOT EXISTS idx_tree_nodes_parent
    ON tree_nodes(parent_id);
