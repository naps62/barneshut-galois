/**
 * Unique ID provider
 */
class UID {
	private:
	static unsigned int nextID;

	public:
	static unsigned int get();
};